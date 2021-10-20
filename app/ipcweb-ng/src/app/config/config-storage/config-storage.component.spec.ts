import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigStorageComponent } from './config-storage.component';

describe('ConfigStorageComponent', () => {
  let component: ConfigStorageComponent;
  let fixture: ComponentFixture<ConfigStorageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigStorageComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigStorageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
