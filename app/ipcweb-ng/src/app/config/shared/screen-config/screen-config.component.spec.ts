import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ScreenConfigComponent } from './screen-config.component';

describe('ScreenConfigComponent', () => {
  let component: ScreenConfigComponent;
  let fixture: ComponentFixture<ScreenConfigComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ScreenConfigComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ScreenConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
