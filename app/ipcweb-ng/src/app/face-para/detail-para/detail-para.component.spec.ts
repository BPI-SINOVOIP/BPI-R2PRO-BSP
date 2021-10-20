import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { DetailParaComponent } from './detail-para.component';

describe('DetailParaComponent', () => {
  let component: DetailParaComponent;
  let fixture: ComponentFixture<DetailParaComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ DetailParaComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(DetailParaComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
